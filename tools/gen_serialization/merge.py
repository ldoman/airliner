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

import argparse
from copy import deepcopy
import json
from os.path import exists,join
import sys

def merge_dicts(airliner, ops, msgs):
    merged = airliner.copy()
    
    # Merge in app msgs first
    for app in merged["Airliner"]["apps"]:
        if app in msgs["Airliner"]["apps"]:
            merged["Airliner"]["apps"][app]["operations"] = msgs["Airliner"]["apps"][app]["operations"]
        else:
            print "Warning: {} not found in msgs json".format(app)

    # Merge operational names for fields
    for app in merged["Airliner"]["apps"]:
        if app in ops["Airliner"]["apps"]:
            merged["Airliner"]["apps"][app]["app_ops_name"] = ops["Airliner"]["apps"][app]["app_ops_name"]
            for sym, sym_data in merged["Airliner"]["apps"][app]["proto_msgs"].iteritems():
                sym_data["operational_names"] = ops["Airliner"]["apps"][app]["proto_msgs"][sym]["operational_names"]
        else:
            print "Warning: {} not found in ops json".format(app)
    

    with open("merged.json", "w") as merged_json:
        json.dump(merged, merged_json, indent=4)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--pyliner-json', default='airliner.json',
                        help='The json used by pyliner operations')
    parser.add_argument('--ops', default='pyliner_ops_names.json',
                        help='Json with defined operational names')
    parser.add_argument('--msgs', default='pyliner_msgs.json',
                        help='Json with MsgIds and CmdCodes')
    args = parser.parse_args()

    if exists(args.pyliner_json) and exists(args.ops) and exists(args.msgs):
        with open(args.pyliner_json, "r") as pyliner_json:
            airliner = json.load(pyliner_json)
        with open(args.ops, "r") as ops_json:
            ops = json.load(ops_json)
        with open(args.msgs, "r") as msgs_json:
            msgs = json.load(msgs_json)

        merge_dicts(airliner, ops, msgs)
    else:
        print "All args must exist"
        sys.exit(1)
    

if __name__ == '__main__':
    main()
