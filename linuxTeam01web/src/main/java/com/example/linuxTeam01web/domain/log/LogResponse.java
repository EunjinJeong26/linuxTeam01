package com.example.linuxTeam01web.domain.log;

import lombok.Getter;
import java.util.List;

@Getter
public class LogResponse {
    private final Long logId;
    private final Long userId;
    private final Long teamId;
    private final String session;
    private final Integer cpuUsage;
    private final Double memUsedGb;
    private final Double memTotalGb;
    private final String branch;
    private final String comment;
    private final String createdAt;
    private final List<String> commits;

    public LogResponse(Log log, List<String> commits) {
        this.logId = log.getLogId();
        this.userId = log.getUserId();
        this.teamId = log.getTeamId();
        this.session = log.getSession();
        this.cpuUsage = log.getCpuUsage();
        this.memUsedGb = log.getMemUsedGb();
        this.memTotalGb = log.getMemTotalGb();
        this.branch = log.getBranch();
        this.comment = log.getComment();
        this.createdAt = log.getCreatedAt();
        this.commits = commits;
    }
}