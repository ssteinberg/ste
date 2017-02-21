
Get-ChildItem -Path ./Simulation/src/* -Include *.vert, *.geom, *.frag, *.comp -Recurse | % { .\Simulation\Tools\ste_spirv_compiler\ste_spirv_compiler.exe $_.FullName }
