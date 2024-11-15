import groovy.json.JsonOutput
import groovy.json.JsonSlurper

final JWT_EXPIRATION = 900

GroovyShell shell = new GroovyShell()
def jwt = shell.parse(new File('/opt/imposter/lib/jwt.groovy'))

def jsonSlurper = new JsonSlurper()
def body = jsonSlurper.parseText(context.request.body)
long timestamp = System.currentTimeMillis() / 1000L

def payload = [
    iss: "Wazuh",
    aud: "Wazuh Communications API",
    iat: timestamp,
    exp: timestamp + JWT_EXPIRATION,
    uuid: body.uuid
]

def keyPair = jwt.generateKeyPair()
def token = jwt.generateToken(payload, keyPair)
def output = [token: token]
def jsonOutput = JsonOutput.toJson(output)

respond {
    withContent(jsonOutput)
}
