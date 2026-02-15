#!/usr/bin/python3

#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                   #
#################################################################################################################################################


class DB:
    def __init__(self, Config):
        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import datetime
        from pymongo import MongoClient, errors, ReturnDocument
        from urllib import parse
        from bson.objectid import ObjectId

        self.Config = Config
        self.getcwd = getcwd
        self.join = join
        self.floor = floor
        self.loads = loads
        self.dumps = dumps
        self.dump = dump
        self.datetime = datetime
        self.ObjectId = ObjectId
        self.server = Config.DB_SERVER
        self.port = Config.DB_PORT

        # Ensure username/password are strings (not None), then URL-encode them
        raw_user = Config.DB_USERNAME if Config.DB_USERNAME is not None else ""
        raw_pass = Config.DB_PASSWORD if Config.DB_PASSWORD is not None else ""
        self.username = parse.quote_plus(raw_user)
        self.password = parse.quote_plus(raw_pass)

        self.remoteMongo = MongoClient
        self.ReturnDocument = ReturnDocument
        self.PyMongoError = errors.PyMongoError
        self.BulkWriteError = errors.BulkWriteError

        # Local dev: no TLS
        self.tls = False  # MUST SET TO TRUE IN PRODUCTION

    def __del__(self):
        # Delete class instance to free resources
        pass

    def _mongo_uri(self):
        """
        Return a MongoDB URI.
        - If username/password are empty -> connect without auth (local MongoDB default).
        - Else -> connect with auth.
        """
        if self.username == "" and self.password == "":
            return f"mongodb://{self.server}:{self.port}"
        return f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}"

    ####################
    # LAB 1 FUNCTIONS  #
    ####################

    def addUpdate(self, data):
        """INSERT UPDATE INTO DATABASE"""
        try:
            uri = self._mongo_uri()
            remotedb = self.remoteMongo(uri, tls=self.tls)
            result = remotedb.ELET2415.update.insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ", msg)
            return False
        else:
            return True

    def numberFrequency(self):
        """RETURNS A LIST OF OBJECTS. EACH OBJECT CONTAINS A NUMBER AND ITS FREQUENCY"""
        try:
            uri = self._mongo_uri()
            remotedb = self.remoteMongo(uri, tls=self.tls)
            result = list(
                remotedb.ELET2415.update.aggregate(
                    [
                        {"$group": {"_id": "$number", "frequency": {"$sum": 1}}},
                        {"$sort": {"_id": 1}},
                        {"$project": {"_id": 0, "number": "$_id", "frequency": 1}},
                    ]
                )
            )
        except Exception as e:
            msg = str(e)
            print("numberFrequency error ", msg)
        else:
            return result

    def onCount(self, LED_Name):
        """RETURN A COUNT OF HOW MANY TIME A SPECIFIC LED WAS TURNED ON"""
        try:
            uri = self._mongo_uri()
            remotedb = self.remoteMongo(uri, tls=self.tls)
            result = remotedb.ELET2415.update.count_documents({LED_Name: {"$eq": 1}})
        except Exception as e:
            msg = str(e)
            print("onCount error ", msg)
        else:
            return result


def main():
    from config import Config
    from time import time

    one = DB(Config)

    start = time()
    end = time()
    print(f"completed in: {end - start} seconds")


if __name__ == "__main__":
    main()