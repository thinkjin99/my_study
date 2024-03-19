import java.io.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.*;

public class Server {
	private static final int BUFFER_SIZE = 1024;
	private static final int PORT = 80;

	private Selector selector;
	private ByteBuffer buffer;

	public Server() throws IOException {
		selector = Selector.open();
		buffer = ByteBuffer.allocate(BUFFER_SIZE);
	}

	public void run() throws IOException {
		ServerSocketChannel serverChannel = ServerSocketChannel.open();
		serverChannel.socket().bind(new InetSocketAddress(PORT));
		serverChannel.configureBlocking(false);
		System.out.println(PORT + "포트에 서버 소켓이 할당되었습니다.");

		serverChannel.register(selector, SelectionKey.OP_ACCEPT);

		while (true) {
			int readyChannels = selector.select();
			if (readyChannels == 0) {
				continue;
			}

			Set<SelectionKey> selectedKeys = selector.selectedKeys();
			Iterator<SelectionKey> keyIterator = selectedKeys.iterator();

			while (keyIterator.hasNext()) {
				SelectionKey key = keyIterator.next();

				if (key.isAcceptable()) {
					handleAccept(serverChannel);
				} else if (key.isReadable()) {
					handleRead(key);
				}

				keyIterator.remove();
			}
		}
	}

	private void handleAccept(ServerSocketChannel serverChannel) throws IOException {
		SocketChannel clientChannel = serverChannel.accept();
		clientChannel.configureBlocking(false);
		clientChannel.register(selector, SelectionKey.OP_READ);
	}

	private void handleRead(SelectionKey key) throws IOException {
		SocketChannel clientChannel = (SocketChannel) key.channel();

		buffer.clear();
		int bytesRead = clientChannel.read(buffer);

		if (bytesRead == -1) {
			clientChannel.close();
			key.cancel();
			return;
		}

		buffer.flip();
		byte[] data = new byte[buffer.remaining()];
		buffer.get(data);

		String request = new String(data);
		String[] requestLines = request.split("\n");
		String[] tokenizedStartLine = requestLines[0].split(" ");

		if (tokenizedStartLine[1].equals("/favicon.ico")) {
			return;
		}

		String response = sendResponse(tokenizedStartLine[1]);
		clientChannel.write(ByteBuffer.wrap(response.getBytes()));
		clientChannel.close();
	}

	private String sendResponse(String endpoint) throws IOException {

		StringBuilder fileContent = new StringBuilder();
		addResponseHeader(fileContent);

		if (endpoint.equals("/")) {
			return generateResponse(fileContent, "/article1");
		}

		if (
			endpoint.equals("/article2") ||
			endpoint.equals("/article3")
		) {
			return generateResponse(fileContent, endpoint);
		}

		throw new IllegalArgumentException("요청받은 리소스는 존재하지 않습니다.");
	}

	private void addResponseHeader(StringBuilder fileContent) {
		fileContent.append("""
				HTTP/1.1 200 OK\r
				Content-Type: text/html\r
				\r
				""");
	}

	private String generateResponse(StringBuilder fileContent, String endpoint) throws IOException {
		BufferedReader reader = new BufferedReader(
				new FileReader(String.format("src/resource%s.html", endpoint))
		);

		String line;
		while ((line = reader.readLine()) != null) {
			fileContent.append(line);
		}

		reader.close();

		return fileContent.toString();
	}
}
