#!/usr/bin/env python3
# Use like "python3 tools/prepare.py --base <base game dump> --update <update dump>"

# Keep in mind this build is barebones and mostly for developers to look at closer
# If you have the assets.zip I gave you, you can do "python3 tools/prepare.py --base <base game dump> --update <update dump> [--assets-zip <assets zip>]"

import argparse
import os
import shutil
import zipfile

TOOLS = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(TOOLS)

CONTENT_FOLDERS = ("Common", "music", "WiiU")
ASSETS_DIR = os.path.join(ROOT, "assets")


def get_content_files(dump):
    # "Common/foo.arc" -----> full path of that file in the dump
    content = os.path.join(dump, "content")
    files = {}
    for folder in CONTENT_FOLDERS:
        for directory, _, names in os.walk(os.path.join(content, folder)):
            for name in names:
                full = os.path.join(directory, name)
                files[os.path.relpath(full, content).replace(os.sep, "/")] = full
    return files

#
def main():
    parser = argparse.ArgumentParser(description="Prepare the game files from your own dumps (Wii U specifically)")
    parser.add_argument("--base", required=True, help="extracted BASE game dump")
    parser.add_argument("--update", required=True, help="extracted UPDATE v688 dump")
    parser.add_argument("--assets-zip", help="assets zip to unzip over assets/")
    args = parser.parse_args()

    # Update files over the base
    files = get_content_files(args.base)
    files.update(get_content_files(args.update))
    if os.path.exists(ASSETS_DIR):
        shutil.rmtree(ASSETS_DIR)
    print("copying %d files into assets/" % len(files))
    for relative, source in files.items():
        target = os.path.join(ASSETS_DIR, *relative.split("/"))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copyfile(source, target)

    if args.assets_zip:
        print("unzipping %s" % os.path.basename(args.assets_zip))
        with zipfile.ZipFile(args.assets_zip) as zipped:
            for entry in zipped.infolist():
                if entry.filename.startswith("assets/"):
                    zipped.extract(entry, ROOT)
    print("done")


if __name__ == "__main__":
    main()
