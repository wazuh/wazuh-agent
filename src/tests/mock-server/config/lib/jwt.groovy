import java.security.KeyPair
import java.security.KeyPairGenerator
import java.security.Signature
import java.util.Base64
import groovy.json.JsonOutput

String base64Encode(input) {
    switch (input) {
        case byte[]: return Base64.encoder.encodeToString(input)
            .replace('=', '')
            .replace('+', '-')
            .replace('/', '_')
        case String: return base64Encode(input.bytes)
    }
}

KeyPair generateKeyPair() {
    def keyPairGenerator = KeyPairGenerator.getInstance("EC")
    keyPairGenerator.initialize(256)
    return keyPairGenerator.generateKeyPair()
}

String generateToken(Map payload, KeyPair keyPair) {
    def header = [alg: "ES256", typ: "JWT"]
    def encodedHeader = base64Encode(JsonOutput.toJson(header))
    def encodedPayload = base64Encode(JsonOutput.toJson(payload))

    def signatureInput = "${encodedHeader}.${encodedPayload}".bytes
    def signature = Signature.getInstance("SHA256withECDSA")

    signature.initSign(keyPair.private)
    signature.update(signatureInput)

    def signatureBytes = signature.sign()
    def encodedSignature = base64Encode(signatureBytes)

    return "${encodedHeader}.${encodedPayload}.${encodedSignature}"
}
