package com.example.linuxTeam01web.auth;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;
import org.springframework.stereotype.Component;

import javax.crypto.SecretKey;
import java.util.Date;

@Component
public class JwtUtil {

    // MVP용 하드코딩 시크릿 키 (32바이트 이상이어야 함)
    private static final String SECRET_KEY = "DevlogProjectSecretKeyForMvpTestTokenGeneration";
    private final SecretKey key = Keys.hmacShaKeyFor(SECRET_KEY.getBytes());

    private final long EXPIRATION_TIME = 1000 * 60 * 60 * 24; // 24시간 만료

    public String generateToken(String username) {
        return Jwts.builder()
                .subject(username) // setSubject -> subject 로 변경됨
                .issuedAt(new Date()) // setIssuedAt -> issuedAt 으로 변경됨
                .expiration(new Date(System.currentTimeMillis() + EXPIRATION_TIME)) // setExpiration -> expiration 으로 변경됨
                .signWith(key) // SignatureAlgorithm 파라미터 생략 가능
                .compact();
    }
}