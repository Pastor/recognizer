package ru.voskhod.tesseract.rest;

import org.junit.Test;

import java.io.File;

import static org.junit.Assert.assertNotNull;

public final class ClientImplTest {

    public static final String[] PATHNAMES = {
            "C:\\GitHub\\fetcher\\0D113AC2D95CE7AB1F5819A7173089CFFBA3EE3C740891A9C2A16E2949D544EB.jpg",
            "C:\\GitHub\\fetcher\\0D425759534C8CAB58EAD61C88A1438B163708B22ED0BB7F3360FCA561891E6D.jpg",
            "C:\\GitHub\\fetcher\\1C3E8CB19D18FD1A99B18E58D4BA979A19FCDDD5A2BF975B207E6477190692D0.jpg",
            "C:\\GitHub\\fetcher\\2740E61AF2672A8D63862DA90669FB35C7C7E61D534F9A374AB02D322C596101.png",
            "C:\\GitHub\\fetcher\\2EC780121A18D57C48214EAE7D8076B8191F2982FEB6BAB328332DA922906425.jpg",
            "C:\\GitHub\\fetcher\\403295B35D2138D5F85F5AEB22C0FAF2A1829FC15DC4258F9D4479AAFBBE0269.jpg",
            "C:\\GitHub\\fetcher\\46E49B3E9A9D14ABA9EF1CEC16C105FD536A6D39A378F46D521B381E623EDB13.jpeg",
            "C:\\GitHub\\fetcher\\69400D14CC65CD83EBCEF0A0BF7730B3DFB23E72D2039310152E8BA63DB61909.jpg"
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
