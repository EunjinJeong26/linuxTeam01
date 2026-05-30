package com.example.linuxTeam01web.domain.log;

import lombok.Getter;
import lombok.NoArgsConstructor;

@Getter
@NoArgsConstructor
public class LogCreateRequest {
    private Long teamId;
    private Double cpuUsage;
    private Double memUsage;
    private String gitCommitHash;
    private String gitCommitMsg;
    private String comment;
}