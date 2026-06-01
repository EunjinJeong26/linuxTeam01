package com.example.linuxTeam01web.team;

import jakarta.persistence.*;
import lombok.Getter;

import java.util.UUID;

@Entity
@Table(name = "teams")
@Getter
public class Team {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String name;

    @Column(nullable = false, unique = true, length = 8)
    private String inviteCode;

    protected Team() {}

    public Team(String name, String inviteCode) {
        this.name = name;
        this.inviteCode = inviteCode;
    }

    public Long getId() { return id; }
    public String getName() { return name; }
    public String getInviteCode() { return inviteCode; }
}
