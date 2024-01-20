import java.util.Scanner;

public class Main {
	private static final String url = "https://www.naver.com/";

	public static void main(String[] args) {
		Scanner sc = new Scanner(System.in);

		System.out.print("요청 횟수를 입력하세요: ");
		int num = sc.nextInt();

		for (int i = 0; i < num; i++) {
			MyThread thread = new MyThread(i, url);
			thread.start();
		}
	}
}