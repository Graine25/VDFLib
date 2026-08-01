"""Dependency-free ctypes bindings for VDFLib's stable C API."""

from __future__ import annotations

import ctypes
import ctypes.util
import os
from pathlib import Path
from typing import Iterator, NamedTuple


class VDFLibError(RuntimeError):
    """An operation reported an error through the VDFLib C API."""


def _load_library() -> ctypes.CDLL:
    configured = os.environ.get("VDFLIB_LIBRARY")
    candidates = [configured] if configured else []
    found = ctypes.util.find_library("vdflib_c")
    if found:
        candidates.append(found)
    package_dir = Path(__file__).parent
    candidates.extend(
        str(package_dir / name)
        for name in ("libvdflib_c.so", "libvdflib_c.dylib", "vdflib_c.dll")
    )
    for candidate in candidates:
        if candidate:
            try:
                return ctypes.CDLL(candidate)
            except OSError:
                pass
    raise ImportError(
        "Could not load vdflib_c. Install the shared library or set "
        "VDFLIB_LIBRARY to its absolute path."
    )


_lib = _load_library()


class _Options(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("exe", ctypes.c_char_p),
        ("start_dir", ctypes.c_char_p),
        ("icon", ctypes.c_char_p),
        ("launch_options", ctypes.c_char_p),
        ("flatpak_app_id", ctypes.c_char_p),
        ("allow_overlay", ctypes.c_uint32),
    ]


_lib.vdflib_last_error.restype = ctypes.c_char_p
_lib.vdflib_generate_shortcut_app_id.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_lib.vdflib_generate_shortcut_app_id.restype = ctypes.c_uint32
_lib.vdflib_repository_create.argtypes = [ctypes.c_char_p]
_lib.vdflib_repository_create.restype = ctypes.c_void_p
_lib.vdflib_repository_destroy.argtypes = [ctypes.c_void_p]
_lib.vdflib_repository_load.argtypes = [ctypes.c_void_p]
_lib.vdflib_repository_load.restype = ctypes.c_int
_lib.vdflib_repository_save.argtypes = [ctypes.c_void_p, ctypes.c_int]
_lib.vdflib_repository_save.restype = ctypes.c_int
_lib.vdflib_repository_count.argtypes = [ctypes.c_void_p]
_lib.vdflib_repository_count.restype = ctypes.c_size_t
_lib.vdflib_repository_app_id.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
_lib.vdflib_repository_app_id.restype = ctypes.c_uint32
_lib.vdflib_repository_name.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
_lib.vdflib_repository_name.restype = ctypes.c_char_p
_lib.vdflib_repository_add.argtypes = [
    ctypes.c_void_p,
    ctypes.POINTER(_Options),
    ctypes.POINTER(ctypes.c_uint32),
]
_lib.vdflib_repository_add.restype = ctypes.c_int
_lib.vdflib_repository_remove.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.vdflib_repository_remove.restype = ctypes.c_int


def _bytes(value: str | os.PathLike[str] | None) -> bytes | None:
    return None if value is None else os.fsencode(value)


def _check(status: int) -> None:
    if status:
        message = _lib.vdflib_last_error()
        raise VDFLibError(
            message.decode("utf-8", errors="replace") if message else "VDFLib error"
        )


def generate_shortcut_app_id(name: str, exe: str | os.PathLike[str]) -> int:
    """Return Steam's app ID for a shortcut name and executable string."""
    result = int(_lib.vdflib_generate_shortcut_app_id(_bytes(name), _bytes(exe)))
    if not result:
        _check(1)
    return result


class ShortcutSummary(NamedTuple):
    app_id: int
    name: str


class ShortcutRepository:
    """An owned shortcuts.vdf repository handle."""

    def __init__(self, path: str | os.PathLike[str]) -> None:
        self.path = Path(path)
        self._handle = _lib.vdflib_repository_create(_bytes(path))
        if not self._handle:
            _check(1)

    def close(self) -> None:
        if self._handle:
            _lib.vdflib_repository_destroy(self._handle)
            self._handle = None

    def __enter__(self) -> "ShortcutRepository":
        return self

    def __exit__(self, *_: object) -> None:
        self.close()

    def __del__(self) -> None:
        self.close()

    def load(self) -> "ShortcutRepository":
        _check(_lib.vdflib_repository_load(self._handle))
        return self

    def save(self, *, backup: bool = True) -> None:
        _check(_lib.vdflib_repository_save(self._handle, backup))

    def __len__(self) -> int:
        return int(_lib.vdflib_repository_count(self._handle))

    def __iter__(self) -> Iterator[ShortcutSummary]:
        for index in range(len(self)):
            name = _lib.vdflib_repository_name(self._handle, index)
            yield ShortcutSummary(
                int(_lib.vdflib_repository_app_id(self._handle, index)),
                name.decode("utf-8") if name else "",
            )

    def add(
        self,
        name: str,
        exe: str | os.PathLike[str],
        start_dir: str | os.PathLike[str],
        *,
        icon: str | os.PathLike[str] | None = None,
        launch_options: str | None = None,
        flatpak_app_id: str | None = None,
        allow_overlay: bool = True,
    ) -> int:
        options = _Options(
            _bytes(name),
            _bytes(exe),
            _bytes(start_dir),
            _bytes(icon),
            _bytes(launch_options),
            _bytes(flatpak_app_id),
            allow_overlay,
        )
        app_id = ctypes.c_uint32()
        _check(_lib.vdflib_repository_add(self._handle, options, ctypes.byref(app_id)))
        return int(app_id.value)

    def remove(self, app_id: int) -> None:
        _check(_lib.vdflib_repository_remove(self._handle, app_id))


__all__ = [
    "ShortcutRepository",
    "ShortcutSummary",
    "VDFLibError",
    "generate_shortcut_app_id",
]
