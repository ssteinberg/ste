param (
	[Parameter(Mandatory=$true)]
	[string] $dir = "."
)

Get-Childitem "${dir}\src\" -recurse -filter "*.hpp" | Copy-Item -Destination "${dir}\include\"