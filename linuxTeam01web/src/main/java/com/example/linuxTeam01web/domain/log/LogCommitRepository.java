package com.example.linuxTeam01web.domain.log;

import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;

public interface LogCommitRepository extends JpaRepository<LogCommit, Long> {
    List<LogCommit> findByLogId(Long logId);
}