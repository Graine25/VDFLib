use std::{env, path::PathBuf, process::Command};

fn main() {
    let crate_dir = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let source = env::var_os("VDFLIB_SOURCE_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            crate_dir
                .ancestors()
                .find(|candidate| candidate.join("CMakeLists.txt").exists())
                .expect("could not find the VDFLib source directory")
                .to_path_buf()
        });
    let out = PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("cmake");
    let status = Command::new("cmake")
        .args(["-S", source.to_str().unwrap(), "-B", out.to_str().unwrap()])
        .arg("-DBUILD_SHARED_LIBS=OFF")
        .arg("-DCMAKE_BUILD_TYPE=Release")
        .status()
        .expect("CMake is required to build VDFLib");
    assert!(status.success(), "VDFLib CMake configuration failed");
    let status = Command::new("cmake")
        .args(["--build", out.to_str().unwrap(), "--config", "Release"])
        .status()
        .expect("failed to invoke CMake build");
    assert!(status.success(), "VDFLib CMake build failed");

    println!("cargo:rustc-link-search=native={}", out.display());
    println!(
        "cargo:rustc-link-search=native={}",
        out.join("Release").display()
    );
    println!("cargo:rustc-link-lib=static=vdflib_c");
    println!("cargo:rustc-link-lib=static=vdflib");
    if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=c++");
    } else if cfg!(target_env = "msvc") {
        // MSVC links its C++ runtime automatically.
    } else {
        println!("cargo:rustc-link-lib=stdc++");
    }
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=advapi32");
    }
    println!("cargo:rerun-if-changed={}", source.join("src").display());
    println!(
        "cargo:rerun-if-changed={}",
        source.join("outsource/cpp").display()
    );
}
