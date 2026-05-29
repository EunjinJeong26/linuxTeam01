package com.example.linuxTeam01web.auth;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;

public interface UserRepository extends JpaRepository<User, Long> {
    boolean existsByUsername(String username);
    // username으로 유저 찾기
    Optional<User> findByUsername(String username);
}