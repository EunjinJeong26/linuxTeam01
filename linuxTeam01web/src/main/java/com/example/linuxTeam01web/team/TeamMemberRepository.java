package com.example.linuxTeam01web.team;

import com.example.linuxTeam01web.auth.User;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface TeamMemberRepository extends JpaRepository<TeamMember, Long> {
    boolean existsByUser(User user);

    long countByTeam(Team team);

    Optional<TeamMember> findByUser(User user);

    List<TeamMember> findByTeam(Team team);
}
