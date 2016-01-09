package ru.voskhod.tesseract.rest;

import java.io.File;
import java.io.IOException;

public interface Client {
    void put(File file) throws IOException;
}
