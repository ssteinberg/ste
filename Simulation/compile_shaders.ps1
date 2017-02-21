param([string]$d=".")

$files = Get-ChildItem -Path $d\src\* -Include *.vert, *.geom, *.frag, *.comp -Recurse 
foreach ($f in $files) {
	& "$d\Tools\ste_spirv_compiler\ste_spirv_compiler.exe" $f.FullName
}
