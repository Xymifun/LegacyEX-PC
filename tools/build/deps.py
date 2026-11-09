"""Fetch the open-source libraries the Wii U build compiles against.

Each library is a pinned upstream release listed in deps.json. Only the part
the build uses is extracted, and a SHA-256 over the extracted files (paths and
contents, with line endings normalised) must match the pin before the files
are used. Downloads are cached, so later builds work offline.

    python tools/build/deps.py <destination>
"""
import hashlib
import io
import json
import os
import shutil
import sys
import tarfile
import urllib.request
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
PINS = os.path.join(HERE, "deps.json")
STAMP = ".deps.json"


class DependencyError(Exception):
    """A dependency could not be downloaded or does not match its pin."""


def tree_hash(files):
    """SHA-256 over sorted (path, content) pairs with CRLF folded to LF."""
    digest = hashlib.sha256()
    for path in sorted(files):
        content = files[path].replace(b"\r\n", b"\n")
        digest.update(path.encode("utf-8") + b"\0" + hashlib.sha256(content).hexdigest().encode("ascii") + b"\n")
    return digest.hexdigest()


def download(url, cache):
    os.makedirs(cache, exist_ok=True)
    path = os.path.join(cache, url.rstrip("/").split("/")[-1])
    if not os.path.isfile(path):
        request = urllib.request.Request(url, headers={"User-Agent": "mcwiiu-reconstructed-build"})
        try:
            with urllib.request.urlopen(request, timeout=600) as response:
                data = response.read()
        except OSError as error:
            raise DependencyError("could not download %s: %s" % (url, error))
        with open(path + ".partial", "wb") as handle:
            handle.write(data)
        os.replace(path + ".partial", path)
    with open(path, "rb") as handle:
        return handle.read()


def archive_members(data, url):
    """Yield (path, bytes) for every regular file in a tar or zip archive."""
    if url.endswith(".zip"):
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            for info in archive.infolist():
                if not info.is_dir():
                    yield info.filename, archive.read(info)
        return
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:*") as archive:
        for member in archive.getmembers():
            if member.isfile():
                yield member.name, archive.extractfile(member).read()


def select(pin, data):
    """The files of one dependency, keyed by their path under its destination."""
    if pin.get("kind") == "file":
        return {os.path.basename(pin["url"]): data}
    strip = pin["strip"].rstrip("/") + "/"
    keep = pin.get("subdir", "").rstrip("/")
    files = {}
    for name, content in archive_members(data, pin["url"]):
        if not name.startswith(strip):
            continue
        relative = name[len(strip):]
        if keep:
            if not relative.startswith(keep + "/"):
                continue
            relative = relative[len(keep) + 1:]
        if relative:
            files[relative] = content
    return files


def fetch(destination, cache=None):
    """Make destination/<dest> for every pinned dependency. Returns the pins."""
    with open(PINS, encoding="utf-8") as handle:
        pins = json.load(handle)
    cache = cache or os.path.join(destination, ".download-cache")
    stamp_path = os.path.join(destination, STAMP)
    done = {}
    if os.path.isfile(stamp_path):
        with open(stamp_path, encoding="utf-8") as handle:
            done = json.load(handle)
    for name, pin in sorted(pins.items()):
        target = os.path.join(destination, pin["dest"])
        if done.get(name) == pin["tree_sha256"] and os.path.isdir(target):
            continue
        print("fetching %s from %s" % (name, pin["url"]), flush=True)
        files = select(pin, download(pin["url"], cache))
        actual = tree_hash(files)
        if actual != pin["tree_sha256"]:
            raise DependencyError("%s from %s does not match its pin (%s, expected %s)"
                                  % (name, pin["url"], actual, pin["tree_sha256"]))
        staging = target + ".partial"
        if os.path.exists(staging):
            shutil.rmtree(staging)
        for relative, content in files.items():
            path = os.path.join(staging, *relative.split("/"))
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "wb") as handle:
                handle.write(content)
        if os.path.exists(target):
            shutil.rmtree(target)
        os.makedirs(os.path.dirname(target) or destination, exist_ok=True)
        os.replace(staging, target)
        done[name] = pin["tree_sha256"]
        with open(stamp_path, "w", encoding="utf-8", newline="\n") as handle:
            json.dump(done, handle, indent=2)
    return pins


def main(argv):
    if len(argv) != 1:
        sys.exit(__doc__)
    try:
        fetch(os.path.abspath(argv[0]))
    except DependencyError as error:
        sys.exit("error: %s" % error)
    print("dependencies ready in %s" % argv[0])


if __name__ == "__main__":
    main(sys.argv[1:])
