package com.example.linuxTeam01web.domain.log;

import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;

@Service
@RequiredArgsConstructor
public class LogService {

    private final LogRepository logRepository;

    // devlog post : 로그 저장
    public Log createLog(String username, Long teamId,
                         Double cpuUsage, Double memUsage,
                         String gitCommitHash, String gitCommitMsg, String comment) {

        // 서버에서 현재 날짜/시간 자동 계산
        LocalDateTime now = LocalDateTime.now();
        String today = now.format(DateTimeFormatter.ofPattern("yyyy-MM-dd"));
        Integer sessionHour = now.getHour(); // 클라이언트에서 안 받고 서버가 직접 계산

        // 같은 슬롯에 이미 로그가 있으면 에러
        logRepository.findByUsernameAndTeamIdAndSessionDateAndSessionHour(
                username, teamId, today, sessionHour)
                .ifPresent(log -> {
                    throw new IllegalStateException("이미 해당 시간에 로그가 존재합니다.");
                });

        // 댓글 150자 초과 체크
        if (comment != null && comment.length() > 150) {
            throw new IllegalArgumentException("댓글은 150자를 초과할 수 없습니다.");
        }

        Log log = new Log();
        log.setUsername(username);
        log.setTeamId(teamId);
        log.setSessionDate(today);
        log.setSessionHour(sessionHour);
        log.setCpuUsage(cpuUsage);
        log.setMemUsage(memUsage);
        log.setGitCommitHash(gitCommitHash);
        log.setGitCommitMsg(gitCommitMsg);
        log.setComment(comment);
        log.setPostedAt(now.format(DateTimeFormatter.ISO_LOCAL_DATE_TIME));

        return logRepository.save(log);
    }

    // devlog show : 팀 전체 로그 조회
    public List<Log> getTeamLogs(Long teamId, String date) {
        return logRepository.findByTeamIdAndSessionDate(teamId, date);
    }

    // devlog show --my : 내 로그만 조회
    public List<Log> getMyLogs(Long teamId, String date, String username) {
        return logRepository.findByTeamIdAndSessionDateAndUsername(teamId, date, username);
    }
}