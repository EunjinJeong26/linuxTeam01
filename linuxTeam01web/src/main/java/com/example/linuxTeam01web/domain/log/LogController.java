package com.example.linuxTeam01web.domain.log;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.bind.annotation.*;
import java.time.LocalDate;
import java.util.List;

@RestController
@RequestMapping("/logs")
@RequiredArgsConstructor
public class LogController {

    private final LogService logService;

    // POST /logs
    @PostMapping
    public ResponseEntity<LogResponse> createLog(@AuthenticationPrincipal Long userId,
                                                 @RequestBody LogCreateRequest request) {
        return ResponseEntity.ok(logService.createLog(userId, request));
    }

    // GET /logs?teamId=1&date=2026-05-08&my=false
    @GetMapping
    public ResponseEntity<List<LogResponse>> getLogs(@AuthenticationPrincipal Long userId, @RequestParam Long teamId,
                                                      @RequestParam(required = false) String date,
                                                      @RequestParam(defaultValue = "false") boolean my) {

        String targetDate = (date != null) ? date : LocalDate.now().toString();

        List<LogResponse> logs = my
                ? logService.getMyLogs(teamId, targetDate, userId)
                : logService.getTeamLogs(teamId, targetDate, userId);

        return ResponseEntity.ok(logs);
    }
}