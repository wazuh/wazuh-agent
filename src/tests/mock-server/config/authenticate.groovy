import groovy.json.JsonOutput
import groovy.json.JsonSlurper

final JWT_EXPIRATION_SECS = 900

def jwt = loadDynamic('/opt/imposter/config/lib/jwt.groovy')

def jsonSlurper = new JsonSlurper()
def body = jsonSlurper.parseText(context.request.body)
long timestamp = System.currentTimeMillis() / 1000L

def payload = [
    iss: "Wazuh",
    aud: "Wazuh Communications API",
    iat: timestamp,
    exp: timestamp + JWT_EXPIRATION_SECS,
    uuid: body.uuid
]

def keyPair = jwt.generateKeyPair()
def token = jwt.generateToken(payload, keyPair)
def output = [token: token]
def jsonOutput = JsonOutput.toJson(output)

respond {
    withContent(jsonOutput)
}
