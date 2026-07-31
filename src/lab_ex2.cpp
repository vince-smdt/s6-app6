#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdlib> // rand
#include <mutex>
#include <queue>
#include <thread>

namespace {
std::queue<int> queue_;
std::mutex mutex_;
std::condition_variable cv_;
std::atomic_bool stop_flag_{};
} // namespace

void add_to_queue(int v) {
  // Fournit un accès synchronisé à queue_ pour l'ajout de valeurs.

  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(v);
}

void prod() {
  // Produit 100 nombres aléatoires de 1000 à 2000 et les ajoute
  // à une file d'attente (queue_) pour traitement.
  // À la fin, transmet "0", ce qui indique que le travail est terminé.

  for (int i = 0; i < 100; ++i) {
    int r = rand() % 1001 + 1000;
    add_to_queue(r);
    cv_.notify_one();

    // Bloque le fil pour 50 ms:
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  stop_flag_ = true;
  cv_.notify_one();
}

void cons() {
  while (!stop_flag_) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock);
    // On doit toujours vérifier si un objet std::queue n'est pas vide
    // avant de retirer un élément.
    while (!queue_.empty()) {
      int v = queue_.front(); // Copie le premier élément de la queue.
      queue_.pop();           // Retire le premier élément.

      printf("Reçu: %d\n", v);
    }
  }
}

int main(int argc, char **argv) {
  std::thread t_prod(prod);
  std::thread t_cons(cons);

  t_prod.join();
  t_cons.join();

  return 0;
}
