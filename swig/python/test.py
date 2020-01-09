#!/usr/bin/python3

import json
import voronota_scripting

sc=voronota_scripting.ScriptingController()
result=sc.run("import structure.pdb")
print(result)

decoded_result=json.loads(result)
print(decoded_result)

print(decoded_result["commands"][0]["output"]["atoms_summary"]["number_total"])
