<#
.SYNOPSIS
    Dumps the content of all files currently staged in git.
    Useful for generating context for LLMs.

.EXAMPLE
    .\dump_staged.ps1 > context.txt
#>

$stagedFiles = (git diff --name-only --cached)

if (-not $stagedFiles) {
    Write-Warning "No staged changes found. Did you run 'git add'?"
    exit
}

foreach ($file in $stagedFiles) {
    Write-Host "================================================================================"
    Write-Host "File: $file"
    Write-Host "================================================================================"
    
    $ext = [System.IO.Path]::GetExtension($file).TrimStart('.')
    if ([string]::IsNullOrWhiteSpace($ext)) { $ext = "text" }
    
    Write-Host "```$ext"
    # Use git show to get the object from the index (staged version)
    # We strip any potential quotes from the filename provided by git
    $gitPath = $file.Trim('"')
    
    # Run git show, redirecting stderr to null to avoid clutter if something weird happens
    git show ":$gitPath" 2>$null
    
    Write-Host "```"
    Write-Host ""
    Write-Host ""
}
