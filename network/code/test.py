import urllib3

ENDPOINT = "http://66.254.114.41"

pool = urllib3.connection_from_url(ENDPOINT)
resp = pool.urlopen("GET", ENDPOINT)
print(resp.data)
