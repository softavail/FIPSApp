package com.example.fipsapp;

import android.content.Context;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {
    static {
        System.loadLibrary("core");
    }

    private EditText urlInput;
    private EditText usernameInput;
    private EditText passwordInput;
    private EditText messageInput;
    private TextView resultText;
    private Button loginButton;
    private Button digestButton;
    private ExecutorService executor;

    // Native methods
    private native boolean initializeFIPS(String fipsConfigPath, String modulesPath);
    private native String computeSHA256(String message);
    private native String performHTTPSGet(String url);

    public static String getFipsConfigPath(Context context) {
        String abi = android.os.Build.SUPPORTED_ABIS[0];

        int resId;
        if (abi.contains("arm64")) {
            resId = R.raw.fips_config_arm64_v8a;
        } else if (abi.contains("armeabi")) {
            resId = R.raw.fips_config_arm64_v8a;
        } else {
            throw new RuntimeException("Unsupported ABI: " + abi);
        }

        File outFile = new File(context.getFilesDir(), "fipsmodule.cnf");

        try (InputStream input = context.getResources().openRawResource(resId);
             OutputStream output = new FileOutputStream(outFile)) {
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to copy FIPS config", e);
        }

        return outFile.getAbsolutePath();
    }

    public static String getFipsModulePath(Context context) {
        String abi = android.os.Build.SUPPORTED_ABIS[0];

        int resId;
        if (abi.contains("arm64")) {
            resId = R.raw.fips_module_arm64_v8a;
        } else if (abi.contains("armeabi")) {
            resId = R.raw.fips_module_arm64_v8a;
        } else {
            throw new RuntimeException("Unsupported ABI: " + abi);
        }

        File outFile = new File(context.getFilesDir(), "fips.so");

        try (InputStream input = context.getResources().openRawResource(resId);
             OutputStream output = new FileOutputStream(outFile)) {
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to copy FIPS config", e);
        }

        return context.getFilesDir().getAbsolutePath();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        executor = Executors.newSingleThreadExecutor();

        urlInput = findViewById(R.id.urlInput);
        usernameInput = findViewById(R.id.usernameInput);
        passwordInput = findViewById(R.id.passwordInput);
        messageInput = findViewById(R.id.messageInput);
        resultText = findViewById(R.id.resultText);
        loginButton = findViewById(R.id.loginButton);
        digestButton = findViewById(R.id.digestButton);

        // Initialize FIPS mode
        executor.execute(() -> {
            String fipsConfigPath = MainActivity.getFipsConfigPath(this);
            String modulesPath = MainActivity.getFipsModulePath(this);
            boolean fipsEnabled = initializeFIPS(fipsConfigPath, modulesPath);
            runOnUiThread(() -> {
                if (fipsEnabled) {
                    Toast.makeText(this, "FIPS mode enabled successfully", Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(this, "Failed to enable FIPS mode", Toast.LENGTH_LONG).show();
                }
            });
        });

        digestButton.setOnClickListener(v -> {
            String message = messageInput.getText().toString();
            if (message.isEmpty()) {
                Toast.makeText(this, "Please enter a message", Toast.LENGTH_SHORT).show();
                return;
            }

            executor.execute(() -> {
                String hash = computeSHA256(message);
                runOnUiThread(() -> {
                    if (hash != null) {
                        resultText.setText("SHA-256 Hash:\n" + hash);
                    } else {
                        resultText.setText("Failed to compute hash");
                    }
                });
            });
        });

        loginButton.setOnClickListener(v -> {
            String url = urlInput.getText().toString();

            if (url.isEmpty()) {
                Toast.makeText(this, "Please fill all fields", Toast.LENGTH_SHORT).show();
                return;
            }

            executor.execute(() -> {
                String response = performHTTPSGet(url);
                runOnUiThread(() -> {
                    if (response != null) {
                        resultText.setText("Response:\n" + response);
                    } else {
                        resultText.setText("HTTP GET failed");
                    }
                });
            });
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (executor != null) {
            executor.shutdown();
        }
    }
}