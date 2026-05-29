package com.example.linuxTeam01web.auth;


// User.java
import jakarta.persistence.*;

@Entity
@Table(name = "users")
public class User {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(unique = true, nullable = false)
    private String username;

    @Column(nullable = false)
    private String password;

    protected User() {} // JPA 기본 생성자

    public User(String username, String password) {
        this.username = username;
        this.password = password; // TODO: 추후 Spring Security 적용 시 암호화 필요
    }

    public Long getId() { return id; }
    public String getUsername() { return username; }
    public String getPassword() { return password; }
}