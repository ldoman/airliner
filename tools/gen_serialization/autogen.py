#!/usr/bin/env python

"""
 
    Copyright (c) 2018 Windhover Labs, L.L.C. All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
 
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. Neither the name Windhover Labs nor the names of its contributors 
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

"""

from cookiecutter.main import cookiecutter
from distutils.dir_util import copy_tree
import json
from jinja2 import Environment, FileSystemLoader
import os
from os.path import join, abspath
from shutil import copyfile, rmtree
import subprocess

extras = {"autogen_version": "0.0.0.1"}

# Get pathing info
base = os.getcwd()
target = join(base, 'src')
apps = join(target, 'apps')
templates = join(base, 'templates')
pyliner_templates = join(base, 'templates-pyliner')
temp = join(base, "temp")

# Remove working dirs if they exist
if os.path.exists(target):
    rmtree(target)
if os.path.exists(temp):
    rmtree(temp)

# Make working dirs
os.mkdir(temp)
os.mkdir(target)
os.mkdir(apps)

with open("cookiecutter.json", 'r') as f:
    cookie_json = json.load(f)

# Apps specific processing
for app, app_data in cookie_json["Airliner"]["apps"].iteritems():
    print "Processing %s..." % app
    app_dir = join(apps, app)
    os.mkdir(app_dir)
    temp = join(base, "temp", app)

    # Need to change current working dir to desired cookiecutter output dir
    os.chdir(app_dir)

    # Need to copy templates into temp dir
    copy_tree(templates, temp)

    app_cookie = app_data
    app_cookie["autogen_version"] = ""
    app_cookie["app_name_lower"] = app.lower()
    app_cookie["_extensions"] = ["jinja2.ext.do", "jinja2.ext.loopcontrols"]

    with open(join(temp, "cookiecutter.json"), "w") as cc:
        json.dump(app_cookie, cc, indent=4)
    
    # Copy proto template file for every proto msg
    for proto_msg, proto_data in app_data["proto_msgs"].iteritems():
        pb_template = join(temp, "template.proto")
        pb_py_template = join(temp, "py_template.proto")
        out_template = join(temp, "{{cookiecutter.autogen_version}}", proto_data["proto_msg"][:-3] + ".proto")
        py_out_template = join(temp, "{{cookiecutter.autogen_version}}", "_py_" + proto_data["proto_msg"][:-3] + ".proto")
        copyfile(pb_template, out_template)
        copyfile(pb_py_template, py_out_template)

    cookiecutter(temp, no_input=True, extra_context=extras, overwrite_if_exists=True)

    # Call protoc and cleanup
    for dir in os.listdir(app_dir):
        for f in os.listdir(dir):
            src = join(app_dir, dir, f)
            dest = join(app_dir, f)
            if f[:4] == "_py_":
                subprocess.call(['protoc', "-I", join(app_dir, dir), '--python_out=' + app_dir, src])
            copyfile(src, dest)
        rmtree(join(app_dir, dir))

    # Organize files
    py_out = join(app_dir, "python_pb")
    py_proto_out = join(app_dir, "protos")
    nano_proto_out = join(app_dir, "protos_nanopb")
    os.mkdir(py_out)
    os.mkdir(py_proto_out)
    os.mkdir(nano_proto_out)
    for f in os.listdir(app_dir):
        src = join(app_dir, f)
        if f[:4] == "_py_":
            if f.split('.')[-1] == "py":
                dest = join(py_out, f[4:])
                copyfile(src, dest)
            if f.split('.')[-1] == "proto":
                dest = join(py_proto_out, f[4:])
                copyfile(src, dest)
            os.remove(src)
        else:
            if f.split('.')[-1] == "proto":
                dest = join(nano_proto_out, f)
                copyfile(src, dest)
                os.remove(src)

print "Finished apps processing. Rolling up results..."

# Consolidate all app protobuf files into rollup directories
nano_protos = join(target, "protos_nanopb")
protos = join(target, "protos")
python_pb = join(target, "python_pb")
os.mkdir(nano_protos)
os.mkdir(protos)
os.mkdir(python_pb)
for root, dirs, files in os.walk(apps):
    if root.split('/')[-1] == "protos_nanopb":
        for f in files:
            path = abspath(join(root, f))
            copyfile(path, join(nano_protos, f))
    if root.split('/')[-1] == "protos":
        for f in files:
            path = abspath(join(root, f))
            copyfile(path, join(protos, f))
    if root.split('/')[-1] == "python_pb":
        for f in files:
            path = abspath(join(root, f))
            copyfile(path, join(python_pb, f))

# Pyliner rollup specific processing
pyliner_temp = join(temp, "pyliner")
os.mkdir(pyliner_temp)
copy_tree(pyliner_templates, pyliner_temp)

# Need to change current working dir to desired cookiecutter output dir
os.chdir(target)

pyliner_cookie = cookie_json
pyliner_cookie["autogen_version"] = ""
pyliner_cookie["_extensions"] = ["jinja2.ext.do", "jinja2.ext.loopcontrols"]

with open(join(pyliner_temp, "cookiecutter.json"), "w") as cc:
    json.dump(pyliner_cookie, cc, indent=4)

cookiecutter(pyliner_temp, no_input=True, extra_context=extras, overwrite_if_exists=True)

pyliner_rollup = join(target, extras["autogen_version"], "all_pyliner_msgs.py")
pyliner_rollup_target = join(python_pb, "pyliner_msgs.py")
copyfile(pyliner_rollup, pyliner_rollup_target)
open(join(python_pb, "__init__.py"), 'a').close()
rmtree(join(target, extras["autogen_version"]))

rmtree(join(base, "temp"))

