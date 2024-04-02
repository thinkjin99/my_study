import sqlite3

filename = (
    "/Users/jin/Library/ApplicationSupport/Google/Chrome/Default/Cookies"  # EDIT ME
)
connection = sqlite3.connect(filename)
cursor = connection.cursor()
cursor.execute("SELECT * FROM cookies;")
results = cursor.fetchall()

for r in results:
    link = r[2]
    data = r[5]
    if len(link):
        print(f"site: {link} value: {data}")


cursor.close()
connection.close()
