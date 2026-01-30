# CuteManga

A manga and comic reader for **Nintendo Switch** (homebrew). Read CBZ comics and image-based manga from your SD card in landscape or portrait, with zoom, fit mode, and cascade viewing.

**Version:** 1.0.5 (see [Releases](https://github.com/pinheiroam/CuteManga/releases) for builds)

---

## Features

- **CBZ support** — Read ZIP-based comic archives (`.cbz`) using MuPDF. High-quality rendering with configurable scale.
- **Image folders** — Browse folders of images (PNG, JPG, etc.) as manga chapters.
- **Portrait & landscape** — Toggle orientation anytime with the **Y** button for comfortable reading.
- **Fit mode** — Scale pages to fit the screen width (toggle with **A**).
- **Zoom** — 2× zoom for detail; pan with the left stick when zoomed (trigger with **ZL**, **ZR**, or **R3**).
- **Cascade mode** — View multiple pages in a vertical strip (enable in the main menu with **X**; uses more memory).
- **Natural sort** — Chapters and pages sorted in natural order (e.g. `page 2` before `page 10`).
- **Keyboard** — On-Switch keyboard for search/input when needed.

---

## How to use

1. **Create a folder** on the root of your SD card named **`CuteManga`**.
2. Put your **CBZ files** and/or **image folders** (manga/comic chapters) inside `CuteManga`.
3. **Folder names** — Avoid Latin/special characters (ñ, á, í, etc.) in folder and file names; use plain ASCII if you have loading issues.
4. **CBZ only** — Only ZIP-based `.cbz` files are supported. RAR-based (`.cbr`) or renamed RAR files will not open; use a ZIP-based CBZ instead.
5. Copy the **`.nro`** build to your Switch (e.g. via SD or your usual homebrew launcher) and run **CuteManga**.

**Path example:** `sdmc:/CuteManga/YourComic.cbz` or `sdmc:/CuteManga/ChapterName/` (folder of images).

---

## Controls

| Action | Button |
|--------|--------|
| **Portrait / Landscape** | **Y** |
| **Previous page** | **L** |
| **Next page** | **R** |
| **Fit mode (on/off)** | **A** |
| **Zoom (2×)** | **ZL** / **ZR** / **R3** |
| **Cascade mode (in reader)** | **X** |
| **Back to list** | **B** |
| **Show / hide shortcuts** | **Minus** or **L3** |
| **Quit** | **Plus** |

In the main menu, **X** enables or disables Cascade Mode for the next time you open a folder (slower load, higher memory use).

---

## Build

- **Requirements:** [devkitPro](https://devkitpro.org/) with **libnx** (Nintendo Switch toolchain). MuPDF is built as part of the project (see `libs/`).
- **Build:** From the project root run `make` (or `makewin.bat` on Windows). Output: `out/CuteManga.nro`.
- **Clean:** `make clean` (and clean MuPDF in `libs/` if you changed MuPDF options).

---

## Credits

- **CuteManga** — Original app by **AngelXex**.
- **MuPDF** — Used for CBZ (ZIP) comic rendering ([Artifex MuPDF](https://mupdf.com/)).
- Nintendo Switch build uses **libnx** and **SDL2** via devkitPro.

---

## Version & releases

- **Version** is set in the project **Makefile** (`VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_MICRO`). The same value is used in the built `.nro` (NACP).
- **Releases** — To publish a release:
  1. Bump the version in the Makefile if needed.
  2. Build: `make` → `out/CuteManga.nro`.
  3. Create a **git tag** (e.g. `v1.0.5`):  
     `git tag -a v1.0.5 -m "Release 1.0.5"`  
     `git push origin v1.0.5`
  4. On GitHub: **Releases** → **Draft a new release** → choose the tag, add release notes, and attach `out/CuteManga.nro` (or a zip with the `.nro`).

---

## License

See [LICENSE](LICENSE) in this repository.
