param([string]$d=".")

$timestamp = "compile_shader_timestamp.temp"
$date = 0
if (Test-Path $timestamp) {
	$date = (Get-Item $timestamp).LastWriteTime
}

$files = Get-ChildItem -Path $d\src\* -Include *.vert, *.geom, *.frag, *.comp -Recurse | Where-Object { $_.LastWriteTime -gt $date }
foreach ($f in $files) {
	& "$d\Tools\ste_spirv_compiler\ste_spirv_compiler.exe" $f.FullName
}

Out-File $timestamp
