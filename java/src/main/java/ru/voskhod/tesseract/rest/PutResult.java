package ru.voskhod.tesseract.rest;

import com.google.common.base.MoreObjects;
import com.google.gson.annotations.SerializedName;

public final class PutResult {
    public String filename;
    @SerializedName("content_type")
    public String contentType;
    @SerializedName("content")
    public String content;
    @SerializedName("created_at")
    public String createdAt;

    @Override
    public String toString() {
        return MoreObjects.toStringHelper(this)
                .add("filename", filename)
                .add("content_type", contentType)
                .add("content", content)
                .add("created_at", createdAt)
                .toString();
    }
}
