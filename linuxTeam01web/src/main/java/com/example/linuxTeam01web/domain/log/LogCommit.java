package com.example.linuxTeam01web.domain.log;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Entity
@Table(name = "log_commits")
@Getter @Setter @NoArgsConstructor
public class LogCommit {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long commitId;

    private Long logId;

    @Column(nullable = false)
    private String message;
}