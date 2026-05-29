package com.example.linuxTeam01web.auth;

import org.springframework.http.HttpStatus;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

@Service
public class AuthService {

    private final UserRepository userRepository;
    private final JwtUtil jwtUtil;
    private final PasswordEncoder passwordEncoder;

    public AuthService(UserRepository userRepository, JwtUtil jwtUtil, PasswordEncoder passwordEncoder) {
        this.userRepository = userRepository;
        this.jwtUtil = jwtUtil;
        this.passwordEncoder = passwordEncoder;
    }

    @Transactional
    public AuthDto.RegisterResponse register(AuthDto.RegisterRequest request) {
        if (userRepository.existsByUsername(request.username())) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "이미 존재하는 username입니다.");
        }

        // 🌟 비밀번호 암호화(encode) 후 저장
        String encodedPassword = passwordEncoder.encode(request.password());
        User newUser = new User(request.username(), encodedPassword);

        User savedUser = userRepository.save(newUser);
        return new AuthDto.RegisterResponse(savedUser.getId(), savedUser.getUsername());
    }

    @Transactional(readOnly = true)
    public AuthDto.LoginResponse login(AuthDto.LoginRequest request) {
        User user = userRepository.findByUsername(request.username())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "username 또는 password가 올바르지 않습니다."));

        // 🌟 암호화된 비밀번호와 입력된 비밀번호 검증(matches)
        if (!passwordEncoder.matches(request.password(), user.getPassword())) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "username 또는 password가 올바르지 않습니다.");
        }

        String token = jwtUtil.generateToken(user.getUsername());
        return new AuthDto.LoginResponse(token, user.getId(), user.getUsername());
    }
}