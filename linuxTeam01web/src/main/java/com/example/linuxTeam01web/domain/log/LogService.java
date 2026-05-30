package com.example.linuxTeam01web.domain.log;

import com.example.linuxTeam01web.websocket.LogWebSocketHandler;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;

@Service
@RequiredArgsConstructor
public class LogService {

    private final LogRepository logRepository;
    private final LogCommitRepository logCommitRepository;
    private final LogWebSocketHandler logWebSocketHandler;

    @Transactional
    public LogResponse createLog(Long userId, LogCreateRequest request) {

        // 세션 시각 자동 계산 (현재 시간 기준 "yyyy-MM-dd'T'HH:00")
        String session = LocalDateTime.now()
                .format(DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:00"));

        // 같은 슬롯 중복 체크
        logRepository.findByUserIdAndTeamIdAndSession(userId, request.getTeamId(), session)
                .ifPresent(log -> {
                    throw new IllegalStateException("이미 해당 시간에 로그가 존재합니다.");
                });

        // 댓글 150자 초과 체크
        if (request.getComment() != null && request.getComment().length() > 150) {
            throw new IllegalArgumentException("댓글은 150자를 초과할 수 없습니다.");
        }

        // 로그 저장
        Log log = new Log();
        log.setUserId(userId);
        log.setTeamId(request.getTeamId());
        log.setSession(session);
        log.setCpuUsage(request.getCpuUsage());
        log.setMemUsedGb(request.getMemUsedGb());
        log.setMemTotalGb(request.getMemTotalGb());
        log.setBranch(request.getBranch());
        log.setComment(request.getComment());
        log.setCreatedAt(LocalDateTime.now()
                .format(DateTimeFormatter.ISO_LOCAL_DATE_TIME));

        Log saved = logRepository.save(log);

        // 커밋 메시지 저장
        if (request.getCommits() != null) {
            for (String message : request.getCommits()) {
                LogCommit commit = new LogCommit();
                commit.setLogId(saved.getLogId());
                commit.setMessage(message);
                logCommitRepository.save(commit);
            }
        }

        // 커밋 메시지 목록 조회
        List<String> commits = logCommitRepository.findByLogId(saved.getLogId())
                .stream().map(LogCommit::getMessage).toList();

        LogResponse response = new LogResponse(saved, commits);

        // 웹소켓 푸시
        logWebSocketHandler.broadcastLog(saved.getTeamId(), response);

        return response;
    }

    // devlog show --team
    public List<LogResponse> getTeamLogs(Long teamId, String date) {
        return logRepository.findByTeamIdAndSessionStartingWith(teamId, date)
                .stream()
                .map(log -> new LogResponse(log,
                        logCommitRepository.findByLogId(log.getLogId())
                                .stream().map(LogCommit::getMessage).toList()))
                .toList();
    }

    // devlog show --my
    public List<LogResponse> getMyLogs(Long teamId, String date, Long userId) {
        return logRepository.findByTeamIdAndSessionStartingWithAndUserId(teamId, date, userId)
                .stream()
                .map(log -> new LogResponse(log,
                        logCommitRepository.findByLogId(log.getLogId())
                                .stream().map(LogCommit::getMessage).toList()))
                .toList();
    }
}