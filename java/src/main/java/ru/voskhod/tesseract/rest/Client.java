package ru.voskhod.tesseract.rest;

import java.io.File;
import java.io.IOException;

public interface Client {
    PutResult put(File file) throws IOException;
}
