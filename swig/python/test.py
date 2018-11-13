#!/usr/bin/python

import json
import voronota_scripting

sc=voronota_scripting.ScriptingController()
result=sc.run("load /home/kliment/workspace/voronota/tests/input/single/structure.pdb")
print(result)

decoded_result=json.loads(result)
print(decoded_result)

print(decoded_result["commands"][0]["output"]["atoms_summary"]["number_total"])
