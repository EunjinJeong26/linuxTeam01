package com.example.linuxTeam01web.auth;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/auth")
public class AuthController {

    private final AuthService authService;

    public AuthController(AuthService authService) {
        this.authService = authService;
    }

    @PostMapping("/register")
    @ResponseStatus(HttpStatus.CREATED) // 201 상태 코드 반환
    public AuthDto.RegisterResponse register(@RequestBody AuthDto.RegisterRequest request) {
        return authService.register(request);
    }

    @PostMapping("/login")
    @ResponseStatus(HttpStatus.OK) // 200 상태 코드 반환
    public AuthDto.LoginResponse login(@RequestBody AuthDto.LoginRequest request) {
        return authService.login(request);
    }
}