package ru.voskhod.tesseract.rest;

import org.junit.Test;

import java.io.File;

import static org.junit.Assert.*;

public final class ClientImplTest {

    @Test
    public void testPut() throws Exception {
        Client client = new ClientImpl("/rest.properties");
        client.put(new File("D:\\GitHub\\fetcher\\.uncompressed\\CE536BEEE8C305828617E512670648C018DFEC6BEF28C9CC0DFF5AC83E85BF85.jpg"));
    }
}