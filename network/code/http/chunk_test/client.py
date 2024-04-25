from http.client import HTTPConnection
import time
import urllib3


def do_content_length(conn, body):
    data = bytearray()
    for line in body.readlines():
        for b in line:
            data.append(b)
    conn.putheader("Content-Length", f"{len(data)}")
    conn.endheaders()
    conn.send(data)


def chunk_readable(body, blocksize):
    while True:
        datablock = body.read(blocksize)
        if not datablock:
            break
        yield datablock


def do_chunk(conn, body):
    conn.putheader("Transfer-Encoding", "chunked")
    conn.endheaders()

    chunks = chunk_readable(body, 1024)
    for i, chunk in enumerate(chunks):
        print(f"chunking... size: {i * 1024} bytes")
        # if i > 4000:
        # time.sleep(1)
        conn.send(b"%x\r\n%b\r\n" % (len(chunk), chunk))

    conn.send(b"0\r\n\r\n")
    conn.send(body)


# conn = HTTPConnection(host="127.0.0.1", port=80)
# conn.connect()
# conn.putrequest(method="POST", url="/files")


body = open(
    "./test.gif",
    "rb",
)

pool = urllib3.PoolManager()
res = pool.request("POST", url="http://127.0.0.1:80/files", body=body, chunked=False)
print(res.data)
# do_chunk(conn, body)
# do_content_length(conn, body)
# conn.putrequest("GET", url="/")
# print(conn.getresponse().read())
# conn.send(chunk)  # 왜 1바이트 씩 계속 보내는 걸로 구성했지? 청킹 안하면
