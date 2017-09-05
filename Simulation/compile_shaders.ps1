param([string]$d=".")

$d = $d.Trimend('\')

$files = Get-ChildItem -Path $d\src\* -Include *.vert, *.geom, *.frag, *.comp -Recurse
foreach ($f in $files) {
	$params = $params + '"' + $f.FullName + '",'
}

& "$d\Tools\ste_spirv_compiler\ste_spirv_compiler.exe" $d $params
