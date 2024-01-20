import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

public class MyThread extends Thread {
	private final int num;
	private final String url;

	public MyThread(int num, String url) {
		this.num = num;
		this.url = url;
	}
	@Override
	public void run() {
		System.out.printf("%d번 요청이 실행되었습니다.\n", num);
		URL url;

		try {
			url = new URL(this.url);
			HttpURLConnection connection = (HttpURLConnection) url.openConnection();
			connection.setRequestMethod("GET");
			int responseCode = connection.getResponseCode();
			String responseMessage = connection.getResponseMessage();

			System.out.printf("%d번 요청 응답: 상태 코드=%d, 응답 메시지=%s\n", this.num, responseCode, responseMessage);
		} catch (MalformedURLException e) {
			throw new RuntimeException(e.getMessage());
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}
}
