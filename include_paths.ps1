$(Get-ChildItem -Recurse Simulation\src\ | ?{ $_.PSIsContainer } | Resolve-Path -Relative | foreach-object {"`$(SolutionDir)$_"}) -replace "\`$\(SolutionDir\)\.\\", "`$(SolutionDir)" -join "; " 
