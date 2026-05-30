package com.example.linuxTeam01web.team;

import com.example.linuxTeam01web.auth.User;
import jakarta.persistence.*;
import lombok.Getter;

@Entity
@Table(name = "members")
@Getter
public class TeamMember {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "team_id", nullable = false)
    private Team team;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "user_id", nullable = false)
    private User user;

    @Column(nullable = false)
    private String role; // "leader" 또는 "member"

    protected TeamMember() {}

    public TeamMember(Team team, User user, String role) {
        this.team = team;
        this.user = user;
        this.role = role;
    }
}