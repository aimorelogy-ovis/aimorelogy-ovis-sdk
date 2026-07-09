#!/usr/bin/python3
# -*- coding: utf-8 -*-
import logging
import argparse
import os
from array import array
import binascii
from XmlParser import XmlParser
import tempfile
import shutil


MAX_LOAD_SIZE = 16 * 1024 * 1024

CHUNK_TYPE_DONT_CARE = 0
CHUNK_TYPE_CRC_CHECK = 1

FORMAT = "%(levelname)s: %(message)s"
logging.basicConfig(level=logging.INFO, format=FORMAT)


def parse_Args():
    parser = argparse.ArgumentParser(description="Create CVITEK device image partition")

    parser.add_argument(
        "input_dir",
        metavar="input_dir",
        type=str,
        help="the directory you want to pack with cvitek image header",
    )
    parser.add_argument(
        "output_dir",
        metavar="output_folder_path",
        type=str,
        help="the folder path to install dir inclued fip,rootfs and kernel",
    )
    parser.add_argument("xml", help="path to partition xml")
    parser.add_argument(
        "-v", "--verbose", help="increase output verbosity", action="store_true"
    )
    args = parser.parse_args()
    if args.verbose:
        logging.debug("Enable more verbose output")
        logging.getLogger().setLevel(level=logging.DEBUG)

    return args


class ImagerBuilder(object):
    def __init__(self, storage, output_path):
        self.storage = storage
        self.output_path = output_path

    def packHeader(self, part):
        with open(part["file_path"], "rb") as fd:
            magic = fd.read(4)
            if magic == b"RIMG":
                logging.debug("%s has been packed, skip it!" % part["file_name"])
                return
            fd.seek(0)
            Magic = array("b", [ord(c) for c in "RIMG"])
            Version = array("I", [1])
            chunk_header_sz = 64
            Chunk_sz = array("I", [chunk_header_sz])
            chunk_counts = part["file_size"] // MAX_LOAD_SIZE
            remain = part["file_size"] - MAX_LOAD_SIZE * chunk_counts
            if (remain != 0):
                chunk_counts = chunk_counts + 1
            Totak_chunk = array("I", [chunk_counts])
            File_sz = array("I", [part["file_size"] + (chunk_counts * chunk_header_sz)])
            try:
                label = part["label"]
            except KeyError:
                label = "gpt"
            Extra_flags = array("B", [ord(c) for c in label])
            for _ in range(len(label), 32):
                Extra_flags.append(ord("\0"))

            imtb_file = open("imtb", "ab")

            for h in [Magic, Version, Chunk_sz, Totak_chunk, File_sz, Extra_flags]:
                h.tofile(imtb_file)
            total_size = part["file_size"]
            offset = part["offset"]
            part_sz = part["part_size"]
            op_len = 0
            while total_size:
                chunk_sz = min(MAX_LOAD_SIZE, total_size)
                chunk = fd.read(chunk_sz)
                crc = binascii.crc32(chunk) & 0xFFFFFFFF
                if chunk_sz == MAX_LOAD_SIZE:
                    op_len += chunk_sz
                else:
                    op_len = part_sz - op_len
                chunk_header = self._getChunkHeader(chunk_sz, offset, op_len, crc)
                imtb_file.write(chunk_header)
              
                total_size -= chunk_sz
                offset += chunk_sz
          
            imtb_file.flush()
            imtb_file.close()

    def _getChunkHeader(self, size, offset, part_sz, crc32):
        logging.info("size:%x, offset:%x, part_sz:%x, crc:%x" % (size, offset, part_sz, crc32))
        Chunk = array(
            "I",
            [
                CHUNK_TYPE_CRC_CHECK,
                size,
                offset,
                part_sz,
                crc32,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            ],
        )
        return Chunk


def main():
    args = parse_Args()
    xmlParser = XmlParser(args.xml)
    parts = xmlParser.parse(args.input_dir)
    storage = xmlParser.getStorage()
    tmp = tempfile.mkdtemp()
    imgBuilder = ImagerBuilder(storage, tmp)

    imtb_file = os.path.join(os.getcwd(), "imtb")
    if os.path.exists(imtb_file):
        os.remove(imtb_file)

    input_files = [f for f in os.listdir(args.input_dir) if os.path.isfile(os.path.join(args.input_dir, f))]

    for p in parts:
        matching_files = [f for f in input_files if f == p["file_name"]]
        for matched_file in matching_files:
            file_path = os.path.join(args.input_dir, matched_file)
            if (
                storage != "emmc" and storage != "spinor"
                and p["file_size"] > p["part_size"] - 128 * 1024
                and p["mountpoint"]
                and p["mountpoint"] != ""
            ):
                logging.error(
                    "Image {} is too big, it will cause mount partition failed!!".format(matched_file)
                )
                continue
            p["file_path"] = file_path
            imgBuilder.packHeader(p)
            tmp_path = os.path.join(tmp, p["file_name"])
            out_path = os.path.join(args.output_dir, p["file_name"])
            logging.info("Packing %s done!" % (p["file_name"]))

if __name__ == "__main__":
    main()
