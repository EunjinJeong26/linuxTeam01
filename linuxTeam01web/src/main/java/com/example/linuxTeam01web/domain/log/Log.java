package com.example.linuxTeam01web.domain.log;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Entity
@Table(name = "logs", uniqueConstraints = {
    @UniqueConstraint(columnNames = {"username", "teamId", "sessionDate", "sessionHour"})
})
@Getter
@Setter
@NoArgsConstructor
public class Log {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long logId;
    private String username;
    private Long teamId;
    private String sessionDate;
    private Integer sessionHour;
    private Double cpuUsage;
    private Double memUsage;
    private String gitCommitHash;
    private String gitCommitMsg;
    @Column(length = 150)
    private String comment;
    private String postedAt;
}