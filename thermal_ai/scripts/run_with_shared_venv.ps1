param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Script,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ScriptArgs
)

$python = "D:\PracticeProject\thermal_model_tflite\.venv\Scripts\python.exe"
if (-not (Test-Path $python))
{
    throw "Shared Python environment not found: $python"
}

$script_path = $Script
if (-not [System.IO.Path]::IsPathRooted($script_path))
{
    $candidate = Join-Path $PSScriptRoot $script_path
    if (Test-Path $candidate)
    {
        $script_path = $candidate
    }
}

& $python $script_path @ScriptArgs
exit $LASTEXITCODE
