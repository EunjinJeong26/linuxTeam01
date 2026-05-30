package com.example.linuxTeam01web.domain.log;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import java.time.LocalDate;
import java.util.List;

@RestController
@RequestMapping("/logs")
@RequiredArgsConstructor
public class LogController {

    private final LogService logService;

    // POST /logs : devlog post
    @PostMapping
    public ResponseEntity<Log> createLog(@RequestBody LogCreateRequest request,
                                         @RequestHeader("X-Username") String username) {
        Log log = logService.createLog(
                username,
                request.getTeamId(),
                request.getSessionHour(),
                request.getCpuUsage(),
                request.getMemUsage(),
                request.getGitCommitHash(),
                request.getGitCommitMsg(),
                request.getComment()
        );
        return ResponseEntity.ok(new LogResponse(log));
    }

    // GET /logs?date=2026-05-29 : devlog show
    @GetMapping
    public ResponseEntity<List<Log>> getTeamLogs(@RequestParam Long teamId,
                                                  @RequestParam(required = false) String date,
                                                  @RequestHeader("X-Username") String username,
                                                  @RequestParam(defaultValue = "false") boolean my) {
        // date 없으면 오늘 날짜 자동 사용
        String targetDate = (date != null) ? date : LocalDate.now().toString();

        List<Log> logs = my
                ? logService.getMyLogs(teamId, targetDate, username)
                : logService.getTeamLogs(teamId, targetDate);

        // Log 리스트 → LogResponse 리스트로 변환
        List<LogResponse> response = logs.stream()
                .map(LogResponse::new)
                .toList();

        return ResponseEntity.ok(response);

    }
}