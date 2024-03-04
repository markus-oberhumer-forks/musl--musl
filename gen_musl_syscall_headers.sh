#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Copyright (C) Markus Franz Xaver Johannes Oberhumer

export LC_ALL=C.UTF-8
rm -rf ./musl-headers/syscall-linux

# both python2 and python3 work
PYTHON=python3

for f in ./arch/*/bits/syscall.h.in; do
    arch=$(basename $(dirname $(dirname "$f")))
    output=./musl-headers/syscall-linux/$arch/syscall.h
    mkdir -p $(dirname "$output")
    $PYTHON -c '
import re, sys
w = sys.stdout.write
nr_dict = {}
lines = open(sys.argv[1], "r").readlines()
w("/* Code generated automatically; DO NOT EDIT. */\n")
globals = {}
locals = {}
def print_define(a, value):
    if not re.match(r"^\d+$", value):
        value = eval(value, globals, locals)
    value = int(value)
    assert value >= 0
    if value > 8192:
        w("#ifndef %s\n#define %s 0x%x\n#endif\n" % (a, a, value))
    else:
        w("#ifndef %s\n#define %s %d\n#endif\n" % (a, a, value))
    return value
for l in lines:
    l = l.strip()
    l = re.sub(r"\s+", " ", l) # remove tabs
    if not l: continue
    if l.startswith("/*"): continue
    m = re.match(r"^#define\s+(__(NR|NR3264)_\w+) (.*)", l)
    if m:
        a = m.group(1)
        assert a not in nr_dict, ("duplicate key", l)
        value = m.group(3)
        value = re.sub(r"/\*.*", "", value).strip()
        value = print_define(a, value)
        nr_dict[a] = 1
        globals[a] = value
        continue
    m = re.match(r"^#define\s+(__ARM_NR_\w+) (.*)", l)
    if m:
        a = m.group(1)
        value = m.group(2)
        value = print_define(a, value)
        continue
    assert 0, ("bad line", l)
w("\n#ifndef NO_SYS_DEFINES\n")
for a in sorted(nr_dict.keys()): # sort by name
    if re.match(r"^__NR3264_", a): continue
    b = a.replace("__NR_", "SYS_")
    w("#ifndef %s\n#define %s %s\n#endif\n" % (b, b, a))
w("#endif /* NO_SYS_DEFINES */\n")
' $f > $output
done

# verify expected checksums of the 18 generated files
echo '
f31ffa1af6884c2f8a4dbe62870c519cf04aa6ea7441e6daaf7fa0d060d3afff *musl-headers/syscall-linux/aarch64/syscall.h
ac486451286d68d3b4fd86467eef1c9b0c58a2f3170e8b5014d22249f73a29b3 *musl-headers/syscall-linux/arm/syscall.h
0a41fd7a2f3ec9fcc00ded51ba6136f642997b516e5282caebec4057786c13ef *musl-headers/syscall-linux/i386/syscall.h
eeaf2036a7fe076a34410e345ee8ebd0a60b4fe7c9b3e90ade9e7f28c2f46872 *musl-headers/syscall-linux/loongarch64/syscall.h
2bbacea2abeafc0b3fe28221c30b171962f4243a80171af0f26f4a715258a853 *musl-headers/syscall-linux/m68k/syscall.h
6923173189a7f486544a8bda72d698746296d1c4964bdded25b246a318821d0a *musl-headers/syscall-linux/microblaze/syscall.h
739f1de7d8df4c907b4f48ee639708ee8237d096d7d5a332ac85b73a6d723937 *musl-headers/syscall-linux/mips64/syscall.h
17fb98d073a1c591c18648d4acfd35ce1fbec3938e50a74858e5a4f2fc142d24 *musl-headers/syscall-linux/mipsn32/syscall.h
bea057115324a0cfd4ab1306c133872d6b1d94c56f9c5b5ae2680a5406a864bc *musl-headers/syscall-linux/mips/syscall.h
a580f78c362b1d9c332516fb53e0382edb765c301913abbb2cfd31015ab3457e *musl-headers/syscall-linux/or1k/syscall.h
b573740018380a8435a3415e0b721e340165d9e86dfa69d5e7a2872c17d3381d *musl-headers/syscall-linux/powerpc64/syscall.h
5e8056d544765df5a380bbf53e9b2e46b195febb6af98e31ede8e33e9bbd3e06 *musl-headers/syscall-linux/powerpc/syscall.h
319382f883e5ea2d691afc9239a00c4c5ea1a4835060c7429e6576a9cd9cb06b *musl-headers/syscall-linux/riscv32/syscall.h
82a49ee9205b3ede44afb4344242e4778c1704e7a02f3b74d825ff3b9045b5bc *musl-headers/syscall-linux/riscv64/syscall.h
2962285364c2e7319f3bd76d930e76d72b5396f137f7e58908bf145874bd998f *musl-headers/syscall-linux/s390x/syscall.h
c5bbcba9ee5a7af48db81e3eacb5e25f88a470c9e9e88bf9764ae7ec17a90f1b *musl-headers/syscall-linux/sh/syscall.h
466041ae127b619c108d30fbdf0195ad4642b28b5f67b6422eb3edf74c5bbda6 *musl-headers/syscall-linux/x32/syscall.h
59207903477456715ace3f93a64ad7164ab26dd7935b2274324067489b012d76 *musl-headers/syscall-linux/x86_64/syscall.h
' | sed '/^$/d' | sha256sum -c
