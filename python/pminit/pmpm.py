# @file     pmpm.py
#           A minimum copy of npm written in pure Python.
#           Currently, this can only install dependencies specified by package-lock.json into node_modules.
# @author   Tom Tang <xmader@distributive.network>
# @date     July 2023

import json
import io
import os, shutil
import tempfile
import tarfile
from dataclasses import dataclass
import urllib.request
from typing import List, Union

@dataclass
class PackageItem:
    installation_path: str
    tarball_url: str
    has_install_script: bool

def parse_package_lock_json(json_data: Union[str, bytes]) -> List[PackageItem]:
    # See https://docs.npmjs.com/cli/v9/configuring-npm/package-lock-json#packages
    packages: dict = json.loads(json_data)["packages"]
    items: List[PackageItem] = []
    for key, entry in packages.items():
        if key == "":
            # Skip the root project (listed with a key of "")
            continue
        items.append(
            PackageItem(
                installation_path=key, # relative path from the root project folder
                                       # The path is flattened for nested node_modules, e.g., "node_modules/create-ecdh/node_modules/bn.js"
                tarball_url=entry["resolved"], # TODO: handle git dependencies
                has_install_script=entry.get("hasInstallScript", False) # the package has a preinstall, install, or postinstall script
            )
        )
    return items

def download_package(tarball_url: str) -> bytes:
    with urllib.request.urlopen(tarball_url) as response:
        tarball_data: bytes = response.read()
        return tarball_data

def unpack_package(work_dir:str, installation_path: str, tarball_data: bytes):
    installation_path = os.path.join(work_dir, installation_path)
    shutil.rmtree(installation_path, ignore_errors=True)

    with tempfile.TemporaryDirectory(prefix="pmpm_cache-") as tmpdir:
        with io.BytesIO(tarball_data) as tar_file:
            with tarfile.open(fileobj=tar_file) as tar:
                tar.extractall(tmpdir)
        shutil.move(
            os.path.join(tmpdir, "package"), # Strip the root folder
            installation_path
        )

def main(work_dir: str):
    with open(os.path.join(work_dir, "package-lock.json"), encoding="utf-8") as f:
        items = parse_package_lock_json(f.read())
        for i in items:
            print("Installing " + i.installation_path)
            tarball_data = download_package(i.tarball_url)
            unpack_package(work_dir, i.installation_path, tarball_data)

if __name__ == "__main__":
    main(os.getcwd())
