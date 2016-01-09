package ru.voskhod.tesseract.rest;

import com.squareup.okhttp.*;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.CookieManager;
import java.net.CookiePolicy;
import java.nio.file.Files;
import java.util.Properties;
import java.util.concurrent.TimeUnit;

final class ClientImpl implements Client {
    private static final int CONNECT_TIMEOUT_MILLIS = 30 * 1000;
    private static final int READ_TIMEOUT_MILLIS = 30 * 1000;
    private static final int WRITE_TIMEOUT_MILLIS = 30 * 1000;

    private final OkHttpClient client = new OkHttpClient();
    private final HttpUrl url;

    ClientImpl(String resourceSettings) throws IOException {
        init(client);
        try (InputStream stream = ClientImpl.class.getResourceAsStream(resourceSettings)) {
            Properties properties = new Properties();
            properties.load(stream);
            HttpUrl.Builder builder = new HttpUrl.Builder();
            builder.host(properties.getProperty("host"));
            builder.scheme("http");
            String path = properties.getProperty("path");
            if (path != null && !path.isEmpty() && (path.length() != 1 && path.charAt(0) != '/')) {
                builder.addPathSegment(path);
            }
            builder.port(Integer.parseInt(properties.getProperty("port")));
            this.url = builder.build();
        }
    }

    private static void init(OkHttpClient client) {
        client.setConnectTimeout(CONNECT_TIMEOUT_MILLIS, TimeUnit.MILLISECONDS);
        client.setReadTimeout(READ_TIMEOUT_MILLIS, TimeUnit.MILLISECONDS);
        client.setWriteTimeout(WRITE_TIMEOUT_MILLIS, TimeUnit.MILLISECONDS);

        client.setFollowSslRedirects(true);
        client.setFollowRedirects(true);

        CookieManager cookieManager = new CookieManager();
        cookieManager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
        client.setCookieHandler(cookieManager);
        client.setFollowRedirects(false);
    }

    @Override
    public void put(File file) throws IOException {
        HttpUrl url = this.url.newBuilder().addPathSegment("new").build();
        MediaType contentType = mimeType(file);
        RequestBody body = RequestBody.create(contentType, file);
        Request request = new Request.Builder().url(url).post(body).build();
        Call call = client.newCall(request);
        Response execute = call.execute();
        if (!execute.isSuccessful()) {
            throw new IOException(execute.message());
        }
        System.out.println(execute.body().string());
    }

    private static MediaType mimeType(File file) throws IOException {
        return MediaType.parse(Files.probeContentType(file.toPath()));
    }
}
