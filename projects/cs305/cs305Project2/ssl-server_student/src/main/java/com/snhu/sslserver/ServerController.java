package com.snhu.sslserver;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

@RestController
public class ServerController {

    @GetMapping("/hash")
    public String getChecksum() {
        // Step 1: Define the input data
        String data = "Godsgift Arokarawei";  // Replace with your name if needed

        try {
            // Step 2: Initialize SHA-256 MessageDigest
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hashBytes = digest.digest(data.getBytes(StandardCharsets.UTF_8));

            // Step 3: Convert to hex format
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                hexString.append(String.format("%02x", b));
            }

            // Step 4: Return the result
            return "<h2>Original String: " + data + "</h2><br/><h3>SHA-256 Checksum: " + hexString.toString() + "</h3>";

        } catch (NoSuchAlgorithmException e) {
            return "Error generating checksum: " + e.getMessage();
        }
    }
}
