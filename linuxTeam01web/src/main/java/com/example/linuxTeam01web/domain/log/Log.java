package com.example.linuxTeam01web.domain.log;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Entity
@Table(name = "logs", uniqueConstraints = {
    @UniqueConstraint(columnNames = {"user_id", "team_id", "session"})
})
@Getter @Setter @NoArgsConstructor
public class Log {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long logId;

    @Column(name = "user_id", nullable = false)
    private Long userId;

    @Column(name = "team_id", nullable = false)
    private Long teamId;

    @Column(nullable = false)
    private String session;     // "2026-05-08T14:00"

    @Column(nullable = false)
    private Integer cpuUsage;

    @Column(nullable = false)
    private Double memUsedGb;

    @Column(nullable = false)
    private Double memTotalGb;

    private String branch;      // null 가능

    @Column(nullable = false, length = 150)
    private String comment;

    @Column(nullable = false)
    private String createdAt;
}