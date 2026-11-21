"""Builds release/<version>/Left Hand Equipment Overhaul <version>.zip from the built DLL and data/.

Archive layout:
  SKSE/Plugins/LeftHandEquipmentOverhaul.dll
  Meshes/Actors/Character/animations/OpenAnimationReplacer/Left hand equipment swap/...
  LeftHandEquipmentOverhaul_LICENSE/LICENSE.txt, THIRD_PARTY_NOTICES.txt
"""
import hashlib, os, re, sys, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DLL = os.path.join(ROOT, "build", "Release", "LeftHandEquipmentOverhaul.dll")


def version():
    text = open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8").read()
    return re.search(r"project\(LeftHandEquipmentOverhaul VERSION ([\d.]+)\)", text).group(1)


def main():
    ver = version()
    out_dir = os.path.join(ROOT, "release", ver)
    if os.path.exists(out_dir):
        sys.exit(f"{out_dir} exists; move it aside first (it may be an uploaded build)")
    os.makedirs(out_dir)
    zip_path = os.path.join(out_dir, f"Left Hand Equipment Overhaul {ver}.zip")
    entries = [(DLL, "SKSE/Plugins/LeftHandEquipmentOverhaul.dll")]
    data = os.path.join(ROOT, "data")
    for base, _, files in os.walk(data):
        for f in sorted(files):
            p = os.path.join(base, f)
            entries.append((p, os.path.relpath(p, data).replace("\\", "/")))
    for f in ("LICENSE.txt", "THIRD_PARTY_NOTICES.txt"):
        entries.append((os.path.join(ROOT, f), f"LeftHandEquipmentOverhaul_LICENSE/{f}"))
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
        for src, arc in sorted(entries, key=lambda e: e[1]):
            if os.path.getsize(src) == 0:
                sys.exit(f"empty file: {src}")
            z.write(src, arc)
    digest = hashlib.sha256(open(zip_path, "rb").read()).hexdigest().upper()
    open(os.path.join(out_dir, "SHA256.txt"), "w", encoding="ascii").write(f"{digest}  {os.path.basename(zip_path)}\n")
    dll = hashlib.sha256(open(DLL, "rb").read()).hexdigest().upper()
    print(f"{zip_path}\nzip {digest}\ndll {dll}\n{len(entries)} files")


if __name__ == "__main__":
    main()
