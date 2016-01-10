package ru.voskhod.tesseract.rest;

import org.junit.Test;

import java.io.File;

import static org.junit.Assert.assertNotNull;

public final class ClientImplTest {

    public static final String[] PATHNAMES = {
            "D:\\GitHub\\fetcher\\.uncompressed\\CE536BEEE8C305828617E512670648C018DFEC6BEF28C9CC0DFF5AC83E85BF85.jpg",
            "D:\\GitHub\\fetcher\\.uncompressed\\0DB8579AAE75B7127D78EB3FC7E9BF03D5142AF52A648A28223360E87ED889CE.jpg",
            "D:\\GitHub\\fetcher\\.uncompressed\\30E932F62C14DE99D35FB0DD820973206852E96AA46044DEECC7AC78919482A9.jpg",
            "D:\\GitHub\\fetcher\\.uncompressed\\36C411BB83028A31BC660EE0346A53CE0391DE941D66943CD9AC84E224AAD9EE.jpg",
    };

    @Test
    public void testPut() throws Exception {
        Client client = new ClientImpl("/rest.properties");
        for (String pathname : PATHNAMES) {
            PutResult result = client.put(new File(pathname));
            assertNotNull(result);
            System.out.println(result);
        }
    }
}