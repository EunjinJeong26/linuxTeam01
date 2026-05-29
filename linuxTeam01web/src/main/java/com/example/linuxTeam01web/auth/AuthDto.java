package com.example.linuxTeam01web.auth;

public class AuthDto {
    public record RegisterRequest(String username, String password) {}
    public record RegisterResponse(Long user_id, String username) {}

    public record LoginRequest(String username, String password) {}
    public record LoginResponse(String token, Long user_id, String username) {}
}
