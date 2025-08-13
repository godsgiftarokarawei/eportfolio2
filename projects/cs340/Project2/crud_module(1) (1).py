from pymongo import MongoClient
from pymongo.errors import PyMongoError

class CRUD:
    def __init__(self, uri, db_name, user, password):
        try:
            self.client = MongoClient(uri, username=user, password=password)
            self.db = self.client[db_name]
        except PyMongoError as e:
            print(f"Error connecting to MongoDB: {e}")

    def create(self, collection_name, document):
        try:
            collection = self.db[collection_name]
            result = collection.insert_one(document)
            return True if result.inserted_id else False
        except PyMongoError as e:
            print(f"Create operation failed: {e}")
            return False

    def read(self, collection_name, query):
        try:
            collection = self.db[collection_name]
            cursor = collection.find(query)
            return list(cursor)
        except PyMongoError as e:
            print(f"Read operation failed: {e}")
            return []

    def update(self, collection_name, query, update_values):
        try:
            collection = self.db[collection_name]
            result = collection.update_many(query, {'$set': update_values})
            return result.modified_count
        except PyMongoError as e:
            print(f"Update operation failed: {e}")
            return 0

    def delete(self, collection_name, query):
        try:
            collection = self.db[collection_name]
            result = collection.delete_many(query)
            return result.deleted_count
        except PyMongoError as e:
            print(f"Delete operation failed: {e}")
            return 0
