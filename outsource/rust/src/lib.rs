//! Safe, dependency-free Rust bindings to VDFLib.

use std::{
    ffi::{c_char, c_int, c_void, CStr, CString, OsStr},
    fmt, io,
    os::raw::c_uint,
    path::Path,
    ptr::NonNull,
};

#[repr(C)]
struct Options {
    name: *const c_char,
    exe: *const c_char,
    start_dir: *const c_char,
    icon: *const c_char,
    launch_options: *const c_char,
    flatpak_app_id: *const c_char,
    allow_overlay: c_uint,
}

unsafe extern "C" {
    fn vdflib_last_error() -> *const c_char;
    fn vdflib_generate_shortcut_app_id(name: *const c_char, exe: *const c_char) -> c_uint;
    fn vdflib_repository_create(path: *const c_char) -> *mut c_void;
    fn vdflib_repository_destroy(repository: *mut c_void);
    fn vdflib_repository_load(repository: *mut c_void) -> c_int;
    fn vdflib_repository_save(repository: *mut c_void, backup: c_int) -> c_int;
    fn vdflib_repository_count(repository: *const c_void) -> usize;
    fn vdflib_repository_app_id(repository: *const c_void, index: usize) -> c_uint;
    fn vdflib_repository_name(repository: *const c_void, index: usize) -> *const c_char;
    fn vdflib_repository_add(
        repository: *mut c_void,
        options: *const Options,
        app_id: *mut c_uint,
    ) -> c_int;
    fn vdflib_repository_remove(repository: *mut c_void, app_id: c_uint) -> c_int;
}

#[derive(Debug, Clone)]
pub struct Error(String);

impl fmt::Display for Error {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(&self.0)
    }
}

impl std::error::Error for Error {}

fn last_error() -> Error {
    unsafe {
        let value = vdflib_last_error();
        Error(if value.is_null() {
            "VDFLib error".into()
        } else {
            CStr::from_ptr(value).to_string_lossy().into_owned()
        })
    }
}

fn check(status: c_int) -> Result<(), Error> {
    if status == 0 {
        Ok(())
    } else {
        Err(last_error())
    }
}

fn string(value: impl AsRef<OsStr>) -> Result<CString, Error> {
    CString::new(value.as_ref().to_string_lossy().as_bytes())
        .map_err(|_| Error("path or string contains a NUL byte".into()))
}

pub fn generate_shortcut_app_id(name: &str, exe: &Path) -> Result<u32, Error> {
    let name = string(name)?;
    let exe = string(exe)?;
    let result = unsafe { vdflib_generate_shortcut_app_id(name.as_ptr(), exe.as_ptr()) };
    if result == 0 {
        Err(last_error())
    } else {
        Ok(result)
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShortcutSummary {
    pub app_id: u32,
    pub name: String,
}

pub struct ShortcutRepository {
    handle: NonNull<c_void>,
}

impl ShortcutRepository {
    pub fn new(path: impl AsRef<Path>) -> Result<Self, Error> {
        let path = string(path.as_ref())?;
        let handle = NonNull::new(unsafe { vdflib_repository_create(path.as_ptr()) })
            .ok_or_else(last_error)?;
        Ok(Self { handle })
    }

    pub fn load(&mut self) -> Result<&mut Self, Error> {
        check(unsafe { vdflib_repository_load(self.handle.as_ptr()) })?;
        Ok(self)
    }

    pub fn save(&self, backup: bool) -> Result<(), Error> {
        check(unsafe { vdflib_repository_save(self.handle.as_ptr(), backup.into()) })
    }

    pub fn len(&self) -> usize {
        unsafe { vdflib_repository_count(self.handle.as_ptr()) }
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    pub fn shortcuts(&self) -> Vec<ShortcutSummary> {
        (0..self.len())
            .map(|index| unsafe {
                let name = vdflib_repository_name(self.handle.as_ptr(), index);
                ShortcutSummary {
                    app_id: vdflib_repository_app_id(self.handle.as_ptr(), index),
                    name: if name.is_null() {
                        String::new()
                    } else {
                        CStr::from_ptr(name).to_string_lossy().into_owned()
                    },
                }
            })
            .collect()
    }

    pub fn add(&mut self, name: &str, exe: &Path, start_dir: &Path) -> Result<u32, Error> {
        let name = string(name)?;
        let exe = string(exe)?;
        let start_dir = string(start_dir)?;
        let options = Options {
            name: name.as_ptr(),
            exe: exe.as_ptr(),
            start_dir: start_dir.as_ptr(),
            icon: std::ptr::null(),
            launch_options: std::ptr::null(),
            flatpak_app_id: std::ptr::null(),
            allow_overlay: 1,
        };
        let mut app_id = 0;
        check(unsafe { vdflib_repository_add(self.handle.as_ptr(), &options, &mut app_id) })?;
        Ok(app_id)
    }

    pub fn remove(&mut self, app_id: u32) -> Result<(), Error> {
        check(unsafe { vdflib_repository_remove(self.handle.as_ptr(), app_id) })
    }
}

impl Drop for ShortcutRepository {
    fn drop(&mut self) {
        unsafe { vdflib_repository_destroy(self.handle.as_ptr()) }
    }
}

unsafe impl Send for ShortcutRepository {}

impl From<Error> for io::Error {
    fn from(value: Error) -> Self {
        io::Error::other(value)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{SystemTime, UNIX_EPOCH};

    #[test]
    fn repository_round_trip_and_backup() {
        let unique = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let directory =
            std::env::temp_dir().join(format!("vdflib-rust-{}-{unique}", std::process::id()));
        let path = directory.join("shortcuts.vdf");

        let mut repository = ShortcutRepository::new(&path).unwrap();
        repository.load().unwrap();
        let app_id = repository
            .add("Rust Test", Path::new("/tmp/rust-test"), Path::new("/tmp"))
            .unwrap();
        repository.save(true).unwrap();
        drop(repository);

        let mut loaded = ShortcutRepository::new(&path).unwrap();
        loaded.load().unwrap();
        assert_eq!(
            loaded.shortcuts(),
            vec![ShortcutSummary {
                app_id,
                name: "Rust Test".into(),
            }]
        );
        loaded.remove(app_id).unwrap();
        loaded.save(true).unwrap();
        assert!(path.with_file_name("shortcuts.vdf.bak").exists());
    }
}
