package com.example.linuxTeam01web.domain.log;

import lombok.Getter;

@Getter
public class LogResponse {
    private final Long logId;
    private final String username;
    private final Long teamId;
    private final String sessionDate;
    private final Integer sessionHour;
    private final Double cpuUsage;
    private final Double memUsage;
    private final String gitCommitHash;
    private final String gitCommitMsg;
    private final String comment;
    private final String postedAt;

    // Log 엔티티를 받아서 Response로 변환
    public LogResponse(Log log) {
        this.logId = log.getLogId();
        this.username = log.getUsername();
        this.teamId = log.getTeamId();
        this.sessionDate = log.getSessionDate();
        this.sessionHour = log.getSessionHour();
        this.cpuUsage = log.getCpuUsage();
        this.memUsage = log.getMemUsage();
        this.gitCommitHash = log.getGitCommitHash();
        this.gitCommitMsg = log.getGitCommitMsg();
        this.comment = log.getComment();
        this.postedAt = log.getPostedAt();
    }
}