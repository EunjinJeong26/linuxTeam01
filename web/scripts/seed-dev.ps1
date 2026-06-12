# Seed data for web frontend M4 verification (server: localhost:8080).
# Prerequisite: linuxTeam01web server running (gradlew.bat bootRun)
# Creates: jin(leader)/minseo accounts, team linux01, 2 logs at current hour.
# NOTE: keep this file ASCII-only (Windows PowerShell 5.1 reads non-BOM files as ANSI).

$base = 'http://localhost:8080'
$pw = 'pass1234'

function Try-Post($uri, $body, $token) {
    $headers = @{}
    if ($token) { $headers['Authorization'] = "Bearer $token" }
    try {
        return Invoke-RestMethod -Method Post -Uri $uri -Headers $headers -ContentType 'application/json; charset=utf-8' -Body $body
    } catch {
        Write-Host "  (skip) $uri : $($_.Exception.Message)"
        return $null
    }
}

Write-Host '[1/5] register jin, minseo'
Try-Post "$base/auth/register" (@{ username = 'jin'; password = $pw } | ConvertTo-Json) $null | Out-Null
Try-Post "$base/auth/register" (@{ username = 'minseo'; password = $pw } | ConvertTo-Json) $null | Out-Null

Write-Host '[2/5] login'
$jin = Invoke-RestMethod -Method Post -Uri "$base/auth/login" -ContentType 'application/json' -Body (@{ username = 'jin'; password = $pw } | ConvertTo-Json)
$minseo = Invoke-RestMethod -Method Post -Uri "$base/auth/login" -ContentType 'application/json' -Body (@{ username = 'minseo'; password = $pw } | ConvertTo-Json)

Write-Host '[3/5] create team (skip if exists)'
Try-Post "$base/teams" (@{ name = 'linux01' } | ConvertTo-Json) $jin.token | Out-Null
$team = Invoke-RestMethod -Uri "$base/teams/me" -Headers @{ Authorization = "Bearer $($jin.token)" }
Write-Host "  team_id=$($team.team_id) invite_code=$($team.invite_code)"

Write-Host '[4/5] minseo joins team (skip if already member)'
Try-Post "$base/teams/join" (@{ invite_code = $team.invite_code } | ConvertTo-Json) $minseo.token | Out-Null

Write-Host '[5/5] post logs at current hour (skip if duplicate)'
$jinLog = @{
    teamId = $team.team_id; cpuUsage = 42; memUsedGb = 5.2; memTotalGb = 16.0
    branch = 'main'; comment = 'web M4 integration test log (jin)'
    commits = @('feat: web frontend M4 integration', 'fix: mapper field cleanup')
} | ConvertTo-Json
Try-Post "$base/logs" $jinLog $jin.token | Out-Null

$minseoLog = @{
    teamId = $team.team_id; cpuUsage = 77; memUsedGb = 6.1; memTotalGb = 8.0
    branch = $null; comment = 'web M4 integration test log (minseo)'
    commits = @()
} | ConvertTo-Json
Try-Post "$base/logs" $minseoLog $minseo.token | Out-Null

Write-Host ''
Write-Host "Done. Login on the web with jin / $pw  (invite code: $($team.invite_code))"
