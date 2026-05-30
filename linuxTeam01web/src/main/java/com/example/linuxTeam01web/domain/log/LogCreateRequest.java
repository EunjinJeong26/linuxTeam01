package com.example.linuxTeam01web.domain.log;

import lombok.Getter;
import lombok.NoArgsConstructor;
import java.util.List;

@Getter @NoArgsConstructor
public class LogCreateRequest {
    private Long teamId;
    private String session;     // "2026-05-08T14:00"
    private Integer cpuUsage;
    private Double memUsedGb;
    private Double memTotalGb;
    private String branch;
    private String comment;
    private List<String> commits; // 커밋 메시지 목록
}